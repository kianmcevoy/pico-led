#ifndef IDSP_LOOPER_H
#define IDSP_LOOPER_H

#include "idsp/buffer_interface.hpp"
#include "idsp/lookup.hpp"
#include "idsp/std_helpers.hpp"

#ifndef Sample
    #warning `Sample` not defined; defaulting to `float`
    #define Sample float
#endif

namespace idsp
{
    namespace looper
    {

        using IndexT = int;
        using FractionT = Sample;

        struct LoopingParamaters
        {
            FractionT speed;

            bool looping_enabled;

            IndexT loop_start;
            IndexT loop_end;
            IndexT loop_start_fade;
            IndexT loop_end_fade;
            IndexT loop_fade_length;

            IndexT data_start;
            IndexT data_end;
            IndexT data_start_fade;
            IndexT data_end_fade;
            IndexT data_fade_length;
        };

        /** Class for tracking position within a sample buffer or lookup table,
         * split into an integral (`IndexT`) and a fractional (`FractionT`) part. */
        class Position
        {
            public:
                constexpr Position(IndexT i, FractionT f):
                _ind{i}, _frac{f} {}
                constexpr Position(IndexT i):
                Position(i, FractionT(0)) {}
                constexpr Position(FractionT f):
                Position(static_cast<IndexT>(f), f - static_cast<FractionT>(static_cast<IndexT>(f))) {}
                constexpr Position():
                Position(IndexT(0), FractionT(0)) {}

                IDSP_CONSTEXPR_SINCE_CXX20
                ~Position() = default;

                IDSP_CONSTEXPR_SINCE_CXX14
                void set(IndexT i, FractionT f)
                    { *this = Position(i, f); }
                IDSP_CONSTEXPR_SINCE_CXX14
                void set(IndexT i)
                    { *this = Position(i); }
                IDSP_CONSTEXPR_SINCE_CXX14
                void set(FractionT f)
                    { *this = Position(f); }

                IDSP_CONSTEXPR_SINCE_CXX14
                void process(IndexT s)
                {
                    this->_ind += s;
                }
                IDSP_CONSTEXPR_SINCE_CXX14
                void process(FractionT s)
                {
                    this->_frac += s;
                    while (this->_frac >= FractionT(1))
                    {
                        this->_ind++;
                        this->_frac -= FractionT(1);
                    }
                    while (this->_frac < FractionT(0))
                    {
                        this->_ind--;
                        this->_frac += FractionT(1);
                    }
                }

                constexpr IndexT index() const
                    { return this->_ind; }
                constexpr FractionT fraction() const
                    { return this->_frac; }
                constexpr FractionT total() const
                    { return static_cast<FractionT>(this->_ind) + this->_frac; }

            private:
                /** Integral part of the position. */
                IndexT _ind;
                /** Fractional part of the position. */
                FractionT _frac;
        };


        /** Class for tracking progress through a crossfade lookup table. */
        class Fade
        {
            public:
                constexpr Fade():
                _active{false},
                _pos{Position()},
                _ppos{_pos},
                _multiplier{FractionT(1)},
                _polarity{FractionT(1)}
                {}

                IDSP_CONSTEXPR_SINCE_CXX20
                ~Fade() = default;

                /** Sets the Fade to an active state.
                 * @param index Starting index (in the lookup table) of the Fade. */
                void activate(IndexT index);

                /** Sets the Fade to an inactive state.
                 * @note The position data and step size will not be reset until @ref
                 * activate or @ref reset is called. */
                void deactivate();

                /** Resets the Fade to initial conditions. */
                void reset();

                /** Sets the step size multiplier and polarity of the Fade.
                 * The values given to this method will be used to determine how much
                 * the position will move with a call to @ref process – they will be
                 * multiplied together to determine the total step size. */
                void set_step_multiplier(FractionT mult, FractionT pol);

                /** Moves the Fade's position by the given amount, multiplied by the
                 * last value given to @ref setStep. */
                void process(FractionT step);

                /** @return the current step size. */
                constexpr auto multiplier() const
                    { return this->_multiplier; }
                constexpr auto polarity() const
                    { return this->_polarity; }
                /** @return the position index. */
                constexpr auto index() const
                    { return this->_pos.index(); }
                /** @return the previous position index. */
                constexpr auto pindex() const
                    { return this->_ppos.index(); }
                /** @return the position fraction. */
                constexpr auto fraction() const
                    { return this->_pos.fraction(); }
                /** @return the previous position fraction. */
                constexpr auto pfraction() const
                    { return this->_ppos.fraction(); }
                /** @return the total position. */
                constexpr auto total() const
                    { return this->_pos.total(); }
                /** @return the previous total position. */
                constexpr auto ptotal() const
                    { return this->_ppos.total(); }
                /** @return `true` if the Fade is active. */
                constexpr bool is_active() const
                    { return this->_active; }

            private:
                /** State of the Fade. */
                bool _active;
                /** Current position information. */
                Position _pos;
                /** Previous position information. */
                Position _ppos;
                /** Step size multiplier for the Fade. */
                FractionT _multiplier;
                /** Step polarity for the Fade. */
                FractionT _polarity;
        };


        /** Enumeration of fade types. */
        enum class FadeType
        {
            /** Loop fades are those that occur at the start or end of the loop. */
            Loop,
            /** Data fades are those that occur at the start of end of the data, when
             * the position wraps around. */
            Data,
            /** Reset fades are user event driven, such as when a tap is spawned or
             * its position is reset. */
            Reset,
            /** Sync fades are user driven, but indirectly activated, such as for
             * position syncronisation. */
            Sync,
            /** Pause fades occur when a Tap is paused or un-paused. */
            Pause,
            /** Deactivate fades occur when a tap is being faded out smoothly without
             * another tap fading in to replace it. */
            Deactivate,
            /** Kill fades occur when a tap is being quickly faded out without another
             * tap fading in to replace it. */
            Kill,
        };

        /** Number of fade types available. */
        static constexpr size_t num_fade_types = 7;


        /** Size of the crossfade table. */
        static constexpr size_t xfade_table_size = 256;
        /** Crossfade table. */
        extern const idsp::LookupTable<Sample, xfade_table_size> xfade_table;


        /** Class for tracking a position in a sample buffer capable of fading. */
        class FadingPosition
        {
            public:
                /** Constructor. */
                constexpr FadingPosition():
                _active{false},
                _pos{Position()},
                _ppos{_pos},
                _fades{std::array<Fade, num_fade_types>{}}
                {}

                /** Default destructor. */
                IDSP_CONSTEXPR_SINCE_CXX20
                ~FadingPosition() = default;

                /** Sets the FadingPosition to active and sets its position.
                 * @param index starting index. */
                void activate(IndexT index, FractionT frac = 0);

                /** Sets the FadingPosition to an inactive state.
                 * @note The position information is not altered by this method. */
                void deactivate();

                /** Resets the FadingPosition to initial conditions. */
                void reset();

                /** Starts a fade of the specified type and length, from the given
                 * index. The @a fade_in and @a forward parameters are used to determine
                 * the fade position multiplier and polarity.
                 * If the Fade is already active, updates its position multiplier and
                 * polarity.
                 * @param type the type of fade to start.
                 * @param start_pos position index that the fade started.
                 * @param length the length of the fade in samples.
                 * @param fade_in whether the fade is fading in (i.e. from 0 to 1).
                 * @param forward the direction of playback at the time the fade starts. */
                void trigger_fade(FadeType type, IndexT start_pos, size_t length, bool fade_in, bool forward);

                /** Updates the given Fade.
                 * See @ref trigger_fade for details on parameters. */
                void update_fade(FadeType type, size_t length, bool fade_in, bool forward);

                /** Copies another FadingPosition's Fades, except the given Fade, into
                 * this FadingPosition. */
                void copy_fades(const FadingPosition& source, FadeType no_copy_type);

                /** Moves the FadingPosition's position by the given amount.
                 * @param step Scan speed/step. */
                void process(FractionT step);

                /** @return the position index. */
                constexpr auto index() const
                    { return this->_pos.index(); }
                /** @return the previous position index. */
                constexpr auto pindex() const
                    { return this->_ppos.index(); }
                /** @return the position fraction. */
                constexpr auto fraction() const
                    { return this->_pos.fraction(); }
                /** @return the previous position fraction. */
                constexpr auto pfraction() const
                    { return this->_ppos.fraction(); }
                /** @return the total position. */
                constexpr auto total() const
                    { return this->_pos.total(); }
                /** @return the previous total position. */
                constexpr auto ptotal() const
                    { return this->_ppos.total(); }

                /** @return `true` if the FadingPosition is active. */
                constexpr bool is_active() const
                    { return this->_active; }

                /** @return `true` if the FadingPosition is currently in any fade. */
                bool is_fading() const;

                /** @return `true` if the FadingPosition is currently in the specifed fade. */
                constexpr bool is_fading(FadeType type) const
                    { return this->_fades[static_cast<size_t>(type)].is_active(); }

                /** Direct read-only access to the array of @ref Fade objects. */
                constexpr const auto& get_fades() const
                    { return this->_fades; }

                /** Applies the crossfade information to the given SampleBuffer.
                 * @note uses the amount last moved and the size of the buffer, so these
                 * should be consistent. */
                void apply_xfade(idsp::BufferInterface& buffer) const;

                /** Updates the FadingPosition's state according to the Fades' states. */
                void update();

            private:
                /** State of the FadingPosition. */
                bool _active;
                /** Current position information. */
                Position _pos;
                /** Previous position information. */
                Position _ppos;
                /** Array of crossfade data. */
                std::array<Fade, num_fade_types> _fades;
        };


        /** Tap class.
         * This manages and represents a user-conceptual 'tap'. */
        class Tap
        {
            public:
                constexpr Tap():
                _fading_pos{},
                _active_fp{},
                _loop_params{}
                {}

                /** Direct access to the underlying FadingPosition container. */
                IDSP_CONSTEXPR_SINCE_CXX14
                auto& list()
                    { return this->_fading_pos; };
                constexpr const auto& list() const
                    { return this->_fading_pos; }

                /** Direct access to the underlying container of active FadingPositions.
                 * @note Statically sized/allocated for performance.
                 * Tap* == nullptr signifies inactivity. */
                IDSP_CONSTEXPR_SINCE_CXX14
                auto& active_list()
                    { return this->_active_fp; }
                constexpr const auto& active_list() const
                    { return this->_active_fp; }

                /** @returns A read-only reference to the Tap's looping parameter data. */
                constexpr const auto& loop_parameters() const
                    { return this->_loop_params; }

                /** Sets the loop parameters of the Tap. */
                void set_loop_parameters(const LoopingParamaters& params);

                /** Activates the Tap.
                 * If the Tap is already active, this will reset the Tap to its starting
                 * position. */
                void activate();

                /** Smoothly moves the Tap to its start position. */
                void restart();

                /** Deactivates the Tap.
                 * Causes all FadingPositions to fade out using the looping crossfade
                 * length. */
                void deactivate();

                /** Kills the Tap.
                 * Causes all FadingPositions to fade out using a short crossfade. */
                void kill();

                /** Pauses the Tap's movement and returns its Positon at time of pause. */
                Position pause();

                /** Unpauses the Tap's movement from @a position. */
                void unpause(const Position& position);

                /** Processes the loop mechanics of the Tap. */
                void process();

                /** Updates the state of the Tap. */
                void update();

                /** Resets the state of the Tap to inactive.
                 * @note This will result in an audible hard cut in audio output if the
                 * Tap is active at time of calling.
                 */
                void reset();

                /** @returns The current position of the Tap. */
                Position position() const;

                /** @returns The current co-position of the Tap.
                 * Co-position refers to the position of the co-active FadingPosition,
                 * which (if applicable) is the one being faded out to be replaced by
                 * the active FadingPosition. */
                Position coposition() const;

                /** Syncronises this Tap's position to another @ref Position. */
                void syncronise(const Position& pos);

                /** Syncronises this Tap's position and data to another Tap. */
                void syncronise(const Tap& other);

                /** @returns `true` if the Tap is active. */
                bool is_active() const;

            private:
                /** Activates a new @ref FadingPosition.
                 * @param index Starting index of the FadingPosition to be activated.
                 * @return A pointer to the activated FadingPosition. If nullptr, none
                 * was activated. */
                FadingPosition* _activate(IndexT index, FractionT frac = 0);

                /** Fades out all the active FadingPositions. */
                void _fade_out_active(FadeType type, size_t xfade_len);

                static constexpr size_t num_fade_pos = 1 + num_fade_types;

                /** FadingPosition storage. */
                std::array<FadingPosition, num_fade_pos> _fading_pos;

                /** List of active FadingPositions, sorted newest first by activation. */
                std::array<FadingPosition*, num_fade_pos> _active_fp;

                /** Current looping parameters. */
                LoopingParamaters _loop_params;
        };

    } // namespace looper
} // namespace idsp

#endif
